import numpy
import tempfile
import lib.geometry_parser.GeometryFileParser as geom
import re
import sys
import os
import math

from lib.geometry_parser import *

class UnitCell:
    """
    This class is a data structure to store the information of a unit cell. The
    unit of the length of the axis is supposed to be in nm and the unit of the
    angles supposed to be degree.  
    """

    def __init__(self, a, b, c, alpha, beta, gamma):
        """
        Note:
            The centering parameter of the unit cell is not required in the
            constructer but can be set later.

        Args:
            a: The length of the a-axis of the unit cell in nm
            b: The length of the b-axis of the unit cell in nm
            c: The length of the c-axis of the unit cell in nm
            alpha: The length of the alpha angle of the unit cell
            beta: The length of the beta angle of the unit cell
            gamma: The length of the gamma angle of the unit cell
        """
        self.a = a
        self.b = b
        self.c = c
        self.alpha = alpha
        self.beta = beta
        self.gamma = gamma
        self.centering = None


    def to_angstroem(self):
        """
        This method converts the length of the unit cell axis from nm to 
        Angstroem.
        """

        self.a *= 10.0
        self.b *= 10.0
        self.c *= 10.0
    

    def dump(self):
        """
        This method prints the unit cell parameters.
        """

        print("Unit Cell:")
        print("a: ", self.a)
        print("b: ", self.b)
        print("c: ", self.c)
        print("alpha: ", self.alpha)
        print("beta: ", self.beta)
        print("gamma: ", self.gamma)
        print("centering: ", self.centering)


class Crystal:
    """
    This class is a data structure to store the information about a crystal in
    the crystfel stream file.
    """
    
    def __init__(self):
        self.begin_predicted_peaks_pointer = None 
        self.end_predicted_peaks_pointer = None
        self.unit_cell = None
        self.resolution_limit = None


    def dump(self):
        """
        This method prints the parameters of the crystal
        """

        print("Crystal:")
        print("Begin predicted peaks pointer: ", 
            self.begin_predicted_peaks_pointer)
        print("End predicted peaks pointer: ", 
            self.end_predicted_peaks_pointer)
        self.unit_cell.dump()
        print("Resolution limit: ", self.resolution_limit)


class StreamfileParserFlags:
    """
    This class provides flags used in the parsing process of the streamfile.

    Note:
        All the class variables are static and therefore the class should not be
        instantiated.  
    """

    none = 0
    geometry = 1
    chunk = 2
    peak = 3
    predicted_peak = 4
    flag_changed = 5


class LargeFile:
    """
    This class provides the ability to handle large text files relatively
    quickly. This is achieved by storing a table containing the newline
    positions in the file. Hence it is able to quickly jump to a given line
    number without scanning the whole file again. 
    
    Note: 
        The table storing the newline positions is currently deactivated to save
        memory.  

        Most of the methods reimplement the methods of a python file and will
        therefore not be further described.
    """


    def __init__(self, name, mode="r"):
        """
        Args:
            name: Filepath to the file on the harddrive
            mode: Mode in which the file should be opened, e.g. "r","w","rw"
        """

        self.name = name
        self.file = open(name, mode)
        self.length = 0
        """
        self.length = sum(1 for line in self.file)
        self.file.seek(0)
        self._line_offset = numpy.zeros(self.length)
        self._read_file()
        """


    def __exit__(self):
        """
        This method implements the desctructor of the class. The Desctructer
        closes the file when the object is destroyed.
        """

        self.file.close()


    def __iter__(self):
        return self.file


    def __next__(self):
        return self.file.next()


    def _read_file(self):
        """
        This method reads the whole file once and creates the table _line_offset
        which stores the position of the newlines in the file.  
        """

        offset = 0
        for index, line in enumerate(self.file):
            self._line_offset[index] = offset
            offset += len(line)
        self.file.seek(0)


    def close(self):
        self.file.close()


    def tell(self):
        return self.file.tell()


    def seek(self, pos):
        self.file.seek(pos)


    def readline(self):
        return self.file.readline()


    def fileno(self):
        return self.file.fileno()

    """
    def read_line_by_number(self, line_number):
        self.file.seek(self._line_offset[line_number])
        return self.file.readline()
    """
        
        
class Chunk:
    """
    This class is a datastructure to store the information of a Chunk in the
    streamfile.
    """

    def __init__(self, stream_filename, stream_file, clen, clen_codeword):
        """
        Args:
            stream_filename: Filepath to the stream file on the harddrive
            stream_file: File/LargeFile object of the stream file
            clen: value of the clen in the geometry file
            clen_codeword: string under which the clen is stored in the chunk
        """

        # general properties of the chunk
        self.stream_filename = stream_filename
        self.stream_file = stream_file
        self.cxi_filename = None
        self.contained_in_cxi = False
        self.event = None
        self.indexed = False
        self.photon_energy = -1.0
        self.beam_divergence = -1.0
        self.beam_bandwidth = -1.0
        self.clen_codeword = clen_codeword
        self.clen = clen
        self.num_peaks = None 
        self.num_saturated_peaks = None 
        self.first_line = None
        self.last_line = None 
        self.begin_peaks_pointer = None 
        self.end_peaks_pointer = None 
        self.crystals = []
        self.num_crystals = 0

        # implementation specific class variables. Regular expressions
        # are needed for parsing the streamfile
        self._float_matching_pattern = r"""
            [-+]? # optional sign
            (?:
            (?: \d* \. \d+ ) # .1 .12 .123 etc 9.1 etc 98.1 etc
            |
            (?: \d+ \.? ) # 1. 12. 123. etc 1 12 123 etc
            )
            # followed by optional exponent part if desired
            (?: [Ee] [+-]? \d+ ) ?
        """
        self._integer_matching_pattern = r"""
            (?<![-.])\b[0-9]{1,}\b(?!\.[0-9])
        """

        self._float_matching_regex = re.compile(
            self._float_matching_pattern, re.VERBOSE)

        self._integer_matching_regex = re.compile(
            self._integer_matching_pattern, re.VERBOSE)

        self._flag = StreamfileParserFlags.none


    def dump(self):
        """
        This method prints all the Chunk information.
        """

        print("""-----------------------------------------------------------""")
        print("Dumping all the chunk information.")
        print("Filename: ", self.cxi_filename)
        print("Event: ", self.event)
        print("Indexed: ", self.indexed)
        print("Photon Energy [eV]: ", self.photon_energy)
        print("Beam divergence [rad]: ", self.beam_divergence)
        print("Average camera length [m]: ", self.average_camera_length)
        print("Num peaks: ", self.num_peaks)
        print("Num saturated peaks: ", self.num_saturated_peaks)
        print("First line: ", self.first_line)
        print("Last line: ", self.last_line)
        print("Crystal found: ", self.has_crystal())
        print("Number of crystals found: ", len(self.crystals))
        print("First peaks line: ", self.begin_peaks_pointer)
        print("Last peaks line: ", self.end_peaks_pointer)


    def parse_line(self, line, previous_line_pointer, current_line_pointer, 
        next_line_pointer):
        """
        This methods parses one line of the stream file. The information in the
        line is stored in the corresponding Chunk variables. 

        Args:
            line: String containing the the line of the streamfile which shall
                be parsed.
            previous_line_pointer: Integer storing the beginning position of
                the previous line in the stream file.
            current_line_pointer: Integer storing the beginning position of
                the current line in the stream file.
            next_line_pointer: Integer storing the beginning position of the 
                next line in the stream file.


        Note:
            The information of the beginning positions of the previous, current
            and next line is needed to read information from the stream file
            after the parsing process. With the method seek of a python file it
            is possible to set the file pointer to the desired position and
            read from there. This is really fast and used to read crystal and
            predicted peak information when needed from the streamfile.
        """

        if self._flag == StreamfileParserFlags.peak:
            if "fs/px" in line:
                self.begin_peaks_pointer = next_line_pointer

        if self._flag == StreamfileParserFlags.predicted_peak:
            if "fs/px" in line:
                self.crystals[self.num_crystals - 1]. \
                    begin_predicted_peaks_pointer = next_line_pointer
        if self.clen_codeword is not None:
            if self.clen_codeword in line:
                # we split the line first because the clen_codeword may contain
                # some float number
                search_string = line.split("=")[1]
                matches = re.findall(
                    self._float_matching_regex, search_string)
                if matches:
                    self.clen = float(matches[0])
                else:
                    self.clen = float('nan')

        if "Image filename: " in line:
            self.cxi_filename = line.replace("Image filename: ", "").rstrip()
        elif "photon_energy_eV = " in line:
            # there may also be a different "hdf5/.../photon_energy_eV" tag
            # with the in the streamfile. we use just the plain
            # "photon_energy_ev" entry
            if "hdf5" not in line:
                matches = re.findall(
                        self._float_matching_regex, line)
                if matches:
                    self.photon_energy = float(matches[0])
                else:
                    self.photon_energy = float('nan')
        elif "beam_divergence = " in line:
            matches = re.findall(
                    self._float_matching_regex, line)
            if matches:
                self.beam_divergence = float(matches[0])
            else:
                self.beam_divergence = float('nan')
        elif "beam_bandwidth = " in line:
            matches = re.findall(
                    self._float_matching_regex, line)
            if matches:
                self.beam_bandwidth = float(matches[0])
            else:
                self.beam_bandwidth = float('nan')
        elif "num_peaks = " in line:
            self.num_peaks = int(line.replace("num_peaks = ", ""))
        elif "num_saturated_peaks = " in line:
            self.num_saturated_peaks = int(
                line.replace("num_saturated_peaks = ", ""))
        elif "indexed_by" in line:
            if not "indexed_by = none" in line:
                self.indexed = True
        elif "Event: " in line:
            self.event = int(re.findall(self._integer_matching_regex, line)[0])
        # It would maybe be a better style to set the line numbers from the
        # Streamfile class such that the class Chunk has no information about 
        # the current line number. Maybe implement this later.
        elif "Peaks from peak search" in line:
            self._flag = StreamfileParserFlags.peak
            return
        elif "End of peak list" in line:
            self._flag = StreamfileParserFlags.none
            self.end_peaks_pointer = current_line_pointer 
            return
        elif "Begin crystal" in line:
            self.num_crystals += 1
            self.crystals.append(Crystal())
            #self.crystal = True
        elif "Reflections measured after indexing" in line:
            self._flag = StreamfileParserFlags.predicted_peak
            return
        elif "End of reflections" in line:
            self._flag = StreamfileParserFlags.none
            self.crystals[self.num_crystals - 1].end_predicted_peaks_pointer = \
                current_line_pointer
            return
        elif "Cell parameters" in line:
            self.crystals[self.num_crystals-1].unit_cell = UnitCell(
                float(re.findall(self._float_matching_regex, line)[0]),
                float(re.findall(self._float_matching_regex, line)[1]),
                float(re.findall(self._float_matching_regex, line)[2]),
                float(re.findall(self._float_matching_regex, line)[3]),
                float(re.findall(self._float_matching_regex, line)[4]),
                float(re.findall(self._float_matching_regex, line)[5]))
            self.crystals[self.num_crystals - 1].unit_cell.to_angstroem()
        elif "centering = " in line:
            self.crystals[self.num_crystals - 1].unit_cell.centering = \
                line.replace("centering = ", "").strip() 
        elif "diffraction_resolution_limit" in line:
            # we use the second resolution index in angstroem
            matches = re.findall(
               self._float_matching_regex, line)
            if matches:
                self.crystals[self.num_crystals - 1].resolution_limit = \
                    float(matches[2])


    def _get_coordinates_from_streamfile(self, begin_pointer, end_pointer, 
        x_column, y_column):
        try:
            line_number = 1
            peak_x_data = []
            peak_y_data = []

            self.stream_file.seek(begin_pointer)
            while(self.stream_file.tell() != end_pointer):
                line = self.stream_file.readline()
                matches = re.findall(self._float_matching_regex, line)
                # TODO: look up if fs is always x in the cheetah 
                # implementation
                peak_x_data.append(float(matches[x_column]))
                peak_y_data.append(float(matches[y_column]))

            return (peak_x_data, peak_y_data)
        except IOError:
            print("Cannot read the peak information from streamfile: ", 
                self.filename)
            return ([], [])


    def get_hkl_indices_from_streamfile(self, crystal_index, peak_x, peak_y):
        """
        This method returns the hkl indices of the predicted bragg peaks of
        the crystal with the given index at the position (peak_x, peak_y)

        Args:
            crystal_index (int): Index of the crystal from which the unit cell
                is returned.
            peak_x (float): The x coordinate of the peak position
            peak_y (float): The y coordinate of the peak position

        Returns:
            list: The hkl indices
        """

        try:
            crystal = self.crystals[crystal_index]
            self.stream_file.seek(crystal.begin_predicted_peaks_pointer)
            while(self.stream_file.tell() != 
                crystal.end_predicted_peaks_pointer):
                line = self.stream_file.readline()
                matches = re.findall(self._float_matching_regex, line)
               
                if(math.isclose(peak_x, float(matches[7]))
                    and math.isclose(peak_y, float(matches[8]))):
                    return [int(matches[0]), int(matches[1]), int(matches[2])]
        except IOError:
            print("Cannot read the peak information from streamfile: ", 
                self.filename)
        return []


    def get_peak_data(self):
        """
        This method returns a list of all the peaks in the chunk of the 
        streamfile.

        Returns:
            list: The list with the positions of the peaks. The peaks are
            stored in the format (x_position, y_position).
        """

        return self._get_coordinates_from_streamfile(self.begin_peaks_pointer,
            self.end_peaks_pointer, 0, 1)


    def has_crystal(self):
        """
        This method returns True if the Chunk has at least one crystal and
        False if no crystal is present.

        Returns:
            Bool
        """

        if (len(self.crystals) != 0):
            return True
        else:
            return False


    def get_number_of_crystals(self):
        """
        This method returns the number of crystals in the chunk.

        Returns:
            number_of_crystals (int): The number of crystals in the chunk
        """
        
        return (len(self.crystals))


    def get_unit_cell(self, crystal_index):
        """
        This method returns the unit cell of the crystal with the given index.

        Args:
            crystal_index (int): Index of the crystal from which the unit cell
                is returned.

        Returns:
            UnitCell: Unit cell of the crystal
        """

        return self.crystals[crystal_index].unit_cell


    def get_predicted_peak_data(self, crystal_index):
        """
        This method returns a list of all the predicted peaks from the crystal
        with the given index.

        Note:
            If the chunk contains no crystal an empty list is returned.

        Args:
            crystal_index (int): Index of the crystal from which the unit cell
                is returned.
                
        Returns:
            list: The list with the positions of the predicted peaks. The peaks 
                are stored in the format (x_position, y_position).
        """
        crystal = self.crystals[crystal_index] 
        return self._get_coordinates_from_streamfile(
                crystal.begin_predicted_peaks_pointer, 
                crystal.end_predicted_peaks_pointer, 7, 8)
        

class Streamfile:
    """
    This class is a datastructure to store the information in the stream file.
    """

    def __init__(self, filename):
        """
        Args:
            filename: Filepath to the stream file on the harddrive
        """

        self.filename = filename

        try: 
            self.file = LargeFile(filename) 
        except IOError:
            print("Cannot read from streamfile: ", self.filename)
            exit()

        self.chunks = []
        # we need a variable to check if the geometry has already been
        # processed because the geometry flag migth appear multiple times
        # in a stream file due to stream concatenation
        self._geometry_processed = False
        self._geom_dict = None
        self.geometry = None
        self._temporary_geometry_file = None
        self._gen_temporary_geometry_file()
        self.parse_streamfile()
        
        self.clen = None
        self.clen_codeword = None


    def __exit__(self, exc_type, exc_value, traceback):
        if not self._temporary_geometry_file.closed:
            self._temporary_geometry_file.close()


    def get_geometry(self):
        """
        This method returns the geometry information in the format requiered
        for the cxiviewer

        Return:
            dict: The geometry information.
        """

        return self._geom_dict


    def get_peak_data(self, index):
        """
        This methods return a list of all the peaks in the streamfile
        corresponding to a specific chunk.

        Args:
            index (int): The number of the chunk from which the peak information
                is returned.

        Returns:
            list: The list with the positions of the peaks. The peaks are
                stored in the format (x_position, y_position).
        """
        return self.chunks[index].get_peak_data()


    def has_crystal(self, index):
        """
        This method returns True or False depending on whether a crystal was
        found in a specific chunk.

        Args:
            index (int): The number of the chunk from which the information
                is returned.

        Returns:
            bool: True if a crystal was found. False if no crystal was found.
        """
        
        return self.chunks[index].has_crystal()

        
    def get_unit_cell(self, index, crystal_index = 0):
        """
        This method returns the UnitCell object of a specific chunk if a
        crystal was found. If no crystal was found None is returned.

        Args:
            index (int): The number of the chunk from which the unit cell
                is returned.

            crystal_index (int): Index of the crystal from which the unit cell
                is returned.

        Returns:
            unit cell (UnitCell): The unit cell of the chunk
        """

        return self.chunks[index].get_unit_cell(crystal_index)


    def get_predicted_peak_data(self, chunk_index, crystal_index = 0):
        """
        This methods return a list of all the predicted peaks in the streamfile
        corresponding to a specific chunk.

        Args:
            index (int): The number of the chunk from which the peak information
                should be returned.

            crystal_index (int): Index of the crystal from which the unit cell
                is returned.

        Returns:
            list: The list with the positions of the predicted peaks. The peaks 
                are stored in the format (x_position, y_position).
        """

        return self.chunks[chunk_index].get_predicted_peak_data(crystal_index)


    def get_number_of_crystals(self, chunk_index):
        """
        This method returns the number of crystals in a specific chunk.
        
        Args:
            index (int): The number of the chunk from which the number of 
                crystals should be returned.

        Returns:
            int: The number of crystals in the chunk
        """
        
        return self.chunks[chunk_index].get_number_of_crystals()


    def get_number_of_chunks(self):
        """
        This methods returns the number of chunks in the streamfile.

        Returns:
            int: The number of chunks
        """

        return len(self.chunks)


    def get_event_id(self, chunk_index):
        """
        This method returns the event id of the chunk.

        Args:
            index (int): The number of the chunk from which the number of 
                crystals should be returned.

        Returns:
            int: The event id of the chunk
        """

        return self.chunks[chunk_index].event


    def get_hkl_indices(self, peak_x, peak_y, chunk_index, crystal_index = 0):
        return self.chunks[chunk_index].get_hkl_indices_from_streamfile(
            crystal_index, peak_x, peak_y)


    def get_cxi_filenames(self):
        list_of_filenames = [] 
        for chunk in self.chunks:
            # TODO: better use a ordered set class here
            if chunk.cxi_filename not in list_of_filenames:
                list_of_filenames.append(chunk.cxi_filename)
        return list_of_filenames


    def get_cxiview_event_list(self):
        """
        This method returns a dictionary with the event information the
        cxiviewer needs to display the events correctly.

        Returns:
            dict: Dictionary containing the event information

        """

        nevents = len(self.chunks)
        filenames = []
        eventids = []
        h5fields = []
        formats = []

        try:
            # TODO: We assume here that the data is stored at the same position
            # for every panel. That may not be true. But currently the cxiviewer
            # does not support different storage positions for different panels
            field = next(iter(self.geometry['panels'].values()))['data']
        except KeyError:
            field = "data/data"

        for chunk in self.chunks:
            filenames.append(chunk.cxi_filename) 
            eventids.append(chunk.event)
            h5fields.append(field)

            basename = os.path.basename(chunk.cxi_filename)
            if basename.endswith(".h5") and basename.startswith("LCLS"):
                formats.append("cheetah_h5")
            elif basename.endswith(".h5"):
                formats.append("generic_h5")
            else:
                formats.append("cxi")

        result = {
            'nevents' : nevents,
            'filename': filenames,
            'event': eventids,
            'h5field': h5fields,
            'format': formats
        }
        return result


    def _gen_temporary_geometry_file(self):
        try:
            self._temporary_geometry_file = tempfile.NamedTemporaryFile(
                mode="w", suffix=".geom", delete = False)
        except IOError:
            print("""Generating the temporary geometry file failed.
                Probably the program does not have the right to write on
                disk.""")
            exit()


    def _write_temporary_geometry_file(self, geometry_lines):
        try:
            for line in geometry_lines: 
                self._temporary_geometry_file.write("%s\n" % line)
            self._temporary_geometry_file.flush()
            self._geometry_processed = True
        except IOError:
            print("""Writing the temporary geometry file failed.
                Probably the program does not have the right to write on
                disk.""")
            exit()


    def parse_streamfile(self):
        """
        This methods parses the stream file. The information in the stream file
        is stored corresponding Streamfile variables.
        """

        # start to parse the streamfile
        # we write the geometry file into a temporary directory to process it
        # with the cheetah function read_geometry in lib/cfel_geometry
        geometry_lines = []
        filenames = []

        flag = StreamfileParserFlags.none
        flag_changed = False
        # line numbering starts at 1
        line_number = 1
        new_chunk = None
        clen = None
        clen_codeword = None

        filesize = os.fstat(self.file.fileno()).st_size
        current_line_pointer = 0
        previous_line_pointer = 0
        next_line_pointer = 0
        try:
            for line in self.file:
                next_line_pointer = current_line_pointer + len(line)

                # scan the line for keywords and perform end or begin flag 
                # operations
                if "Begin geometry file" in line:
                    if not self._geometry_processed:
                        flag = StreamfileParserFlags.geometry
                        flag_changed = True
                elif "End geometry file" in line:
                    if not self._geometry_processed:
                        flag = StreamfileParserFlags.none
                        self._write_temporary_geometry_file(geometry_lines)
                        #self._geom_dict = cfel_geom.read_geometry(
                            #self._temporary_geometry_file.name)
                        geom_parser = geom.GeometryFileParser(
                            self._temporary_geometry_file.name)
                        geom_parser.parse()
                        self.geometry = geom_parser.dictionary
                        self._geom_dict = geom_parser.pixel_map_for_cxiview()

                        # handle the clen property which can be given either as
                        # a float directly or as a codeword in the chunk
                        clen = self._geom_dict['clen']
                        if isinstance(clen, str):
                            clen_codeword = clen
                            clen = None
                           
                        flag_changed = True
                elif "Begin chunk" in line:
                    flag = StreamfileParserFlags.chunk
                    # the clen and clen_codeword parameters has to be passed
                    # from the geometry file to the chunk. the chunk cannot
                    # determine that information by itself
                    new_chunk = Chunk(self.filename, self.file, clen, 
                        clen_codeword)
                    new_chunk.begin_pointer = next_line_pointer
                    flag_changed = True
                elif "End chunk" in line:
                    flag = StreamfileParserFlags.none
                    new_chunk.end_pointer = current_line_pointer
                    self.chunks.append(new_chunk)
                    flag_changed = True


                # Process active flags, python dictionary switch-case
                # alternative doesn't work here because we may need to
                # execute complex tasks depending on the flag which
                # may require many different arguments. Python sucks from
                # time to time.

                # if the flag has changed in the current line there is no
                # more to do
                if not flag_changed:
                    if flag == StreamfileParserFlags.geometry:
                        geometry_lines.append(line.strip())
                    elif flag == StreamfileParserFlags.chunk:
                        new_chunk.parse_line(line, previous_line_pointer, 
                            current_line_pointer, next_line_pointer)
                else:
                    flag_changed = False
                    
                line_number += 1
                previous_line_pointer = current_line_pointer
                current_line_pointer = next_line_pointer
        except IOError:
            print("Cannot read from streamfile: ", self.filename)
            exit()
